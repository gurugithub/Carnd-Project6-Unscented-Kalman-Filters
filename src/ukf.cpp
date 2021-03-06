/**
 Guru Shetti Term 2 Project 2
 */
#include "ukf.h"
#include "Eigen/Dense"
#include <iostream>

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

/**
 * Initializes Unscented Kalman filter
 */
UKF::UKF() {
    // if this is false, laser measurements will be ignored (except during init)
    use_laser_ = true;
    
    // if this is false, radar measurements will be ignored (except during init)
    use_radar_ = true;
    
    // initial state vector
    x_ = VectorXd(5);
    
    // initial covariance matrix
    P_ = MatrixXd(5, 5);
    
    // Process Noise Standard Dev
    // longitudinal acceleration in m/s^2. 30 starting value
    std_a_ = 5;
    
    // yaw acceleration in rad/s^2. 30 starting value
    std_yawdd_ = 0.5;
    
    // Laser measurement position1 in m. 0.15 starting
    std_laspx_ = 0.15;
    
    // Laser measurement position2 in m. 0.15 starting
    std_laspy_ = 0.15;
    
    // Radar measurement radius in m. 0.03 starting
    std_radr_ = 0.3;
    
    // Radar measurement angle in rad. 0.03 starting
    std_radphi_ = 0.03;
    
    // Radar measurement radius change in m/s. 0.3 starting
    std_radrd_ = 0.3;
    
    /**
     TODO:
     
     Complete the initialization. See ukf.h for other member properties.
     
     Hint: one or more values initialized above might be wildly off...
     */
    
    is_initialized_ = false;
    
    //set state dimension
    n_x_ = 5;
    
    //set augmented dimension
    n_aug_ = 7;
    
    //define spreading parameter
    lambda_ = 3 - n_aug_;
    
    // initial state vector
    x_ = VectorXd(n_x_);
    x_ <<  0.0,
    0.0,
    0.0,
    0.0,
    0.0;
    
    // initial covariance matrix
    P_ = MatrixXd(n_x_, n_x_);
    
    
    
    P_ << .0043, 0., 0.,    0.,    0.,
    0., .03, 0.,    0.,    0.,
    0., 0., .5,    0.,    0.,
    0., 0., 0.,    0.13,    0.,
    0., 0., 0.,    0.,    0.23;
    
    //create matrix with predicted sigma points as columns
    Xsig_pred_ = MatrixXd(n_x_, 2 * n_aug_ + 1);
    
    //create vector for weights
    weights_ = VectorXd(2*n_aug_+1);
    
    // end todo
}

UKF::~UKF() {}

/**
 * @param {MeasurementPackage} meas_package The latest measurement data of
 * either radar or laser.
 */
void UKF::ProcessMeasurement(MeasurementPackage meas_package) {
    /**
     TODO:
     
     Complete this function! Make sure you switch between lidar and radar
     measurements.
     */
    if (!is_initialized_) {
        /**
         TODO:
         * Initialize the state x_ with the first measurement.
         * Create the covariance matrix.
         * Remember: you'll need to convert radar from polar to cartesian coordinates.
         */
        // first measurement
        cout << "UKF: " << endl;
        
        if (meas_package.sensor_type_ == MeasurementPackage::RADAR) {
            /**
             Convert radar from polar to cartesian coordinates and initialize state.
             */
            float rho = meas_package.raw_measurements_[0];
            float phi = meas_package.raw_measurements_[1];
            x_ << rho*cos(phi), rho*sin(phi), 0, 0, 0;
        }
        else if (meas_package.sensor_type_ == MeasurementPackage::LASER) {
            
            //We set the state with the initial location and zero velocity
            x_ << meas_package.raw_measurements_[0], meas_package.raw_measurements_[1], 0, 0, 0;
        }
        
        //create matrix with predicted sigma points as columns
        //Xsig_pred_ = MatrixXd(n_x_, 2 * n_aug_ + 1);
        
        //create vector for weights
        //weights_ = VectorXd(2*n_aug_+1);
        
        double weight_0 = lambda_/(lambda_+n_aug_);
        weights_(0) = weight_0;
        for (int i=1; i<2*n_aug_+1; i++) {
            double weight = 0.5/(n_aug_+lambda_);
            weights_(i) = weight;
        }
        
        
        time_us_ = meas_package.timestamp_;
        
        // done initializing, no need to predict or update
        is_initialized_ = true;
        
        return;
    }
    
    double delta_t = (meas_package.timestamp_ - time_us_) / 1000000.0;
    time_us_ = meas_package.timestamp_;
    
    
    Prediction(delta_t);
    
    if (meas_package.sensor_type_ == MeasurementPackage::RADAR && use_radar_) {
        //if (meas_package.sensor_type_ == MeasurementPackage::RADAR) {
        //Prediction(delta_t);
        UpdateRadar(meas_package);
    }
    else if (meas_package.sensor_type_ == MeasurementPackage::LASER && use_laser_) {
        //else if (meas_package.sensor_type_ == MeasurementPackage::LASER) {
        //Prediction(delta_t);
        UpdateLidar(meas_package);
    }
}

/**
 * Predicts sigma points, the state, and the state covariance matrix.
 * @param {double} delta_t the change in time (in seconds) between the last
 * measurement and this one.
 */
void UKF::Prediction(double delta_t) {
    /**
     TODO:
     
     Complete this function! Estimate the object's location. Modify the state
     vector, x_. Predict sigma points, the state, and the state covariance matrix.
     */
    /**
     Complete this function! Estimate the object's location. Modify the state
     vector, x_. Predict sigma points, the state, and the state covariance matrix.
     */
    //create augmented mean vector
    VectorXd x_aug = VectorXd(n_aug_);
    
    //create augmented state covariance
    MatrixXd P_aug = MatrixXd(n_aug_, n_aug_);
    
    //create sigma point matrix
    MatrixXd Xsig_aug = MatrixXd(n_aug_, 2 * n_aug_ + 1);
    
    //std::cout << "IN PREDICT" << std::endl;
    
    //create vector for weights
    //weights_ = VectorXd(2*n_aug_+1);
    
    /**
     classroom.udacity.com/nanodegrees/nd013/parts/40f38239-66b6-46ec-ae68-03afd8a601c8/modules/0949fca6-b379-42af-a919-ee50aa304e6a/lessons/daf3dee8-7117-48e8-a27a-fc4769d2b954/concepts/07444c9b-b4be-4615-96e1-e1b221a9add6
     
     */
    
    //create augmented mean state
    x_aug.head(n_x_) = x_;
    x_aug(5) = 0.0;
    x_aug(6) = 0.0;
    
    //create augmented covariance matrix
    P_aug.fill(0.0);
    P_aug.topLeftCorner(n_x_,n_x_) = P_;
    P_aug(5,5) = std_a_*std_a_;
    P_aug(6,6) = std_yawdd_*std_yawdd_;
    
    //create square root matrix
    MatrixXd L = P_aug.llt().matrixL();
    
    //create augmented sigma points
    Xsig_aug.col(0)  = x_aug;
    for (int i = 0; i< n_aug_; i++)
    {
        Xsig_aug.col(i+1)       = x_aug + sqrt(lambda_+n_aug_) * L.col(i);
        Xsig_aug.col(i+1+n_aug_) = x_aug - sqrt(lambda_+n_aug_) * L.col(i);
    }
    
    /**
     classroom.udacity.com/nanodegrees/nd013/parts/40f38239-66b6-46ec-ae68-03afd8a601c8/modules/0949fca6-b379-42af-a919-ee50aa304e6a/lessons/daf3dee8-7117-48e8-a27a-fc4769d2b954/concepts/63674ce2-43ed-418c-bf8b-d16ae73dffc0
     */
    for (int i = 0; i< 2*n_aug_+1; i++)
    {
        //extract values for better readability
        double p_x = Xsig_aug(0,i);
        double p_y = Xsig_aug(1,i);
        double v = Xsig_aug(2,i);
        double yaw = Xsig_aug(3,i);
        double yawd = Xsig_aug(4,i);
        double nu_a = Xsig_aug(5,i);
        double nu_yawdd = Xsig_aug(6,i);
        
        //predicted state values
        double px_p, py_p;
        
        //avoid division by zero
        if (fabs(yawd) > 0.001) {
            px_p = p_x + v/yawd * ( sin (yaw + yawd*delta_t) - sin(yaw));
            py_p = p_y + v/yawd * ( cos(yaw) - cos(yaw+yawd*delta_t) );
        }
        else {
            px_p = p_x + v*delta_t*cos(yaw);
            py_p = p_y + v*delta_t*sin(yaw);
        }
        
        double v_p = v;
        double yaw_p = yaw + yawd*delta_t;
        double yawd_p = yawd;
        
        //add noise
        px_p = px_p + 0.5*nu_a*delta_t*delta_t * cos(yaw);
        py_p = py_p + 0.5*nu_a*delta_t*delta_t * sin(yaw);
        v_p = v_p + nu_a*delta_t;
        
        yaw_p = yaw_p + 0.5*nu_yawdd*delta_t*delta_t;
        yawd_p = yawd_p + nu_yawdd*delta_t;
        
        //write predicted sigma point into right column
        Xsig_pred_(0,i) = px_p;
        Xsig_pred_(1,i) = py_p;
        Xsig_pred_(2,i) = v_p;
        Xsig_pred_(3,i) = yaw_p;
        Xsig_pred_(4,i) = yawd_p;
    }
    
    
    /*
     classroom.udacity.com/nanodegrees/nd013/parts/40f38239-66b6-46ec-ae68-03afd8a601c8/modules/0949fca6-b379-42af-a919-ee50aa304e6a/lessons/daf3dee8-7117-48e8-a27a-fc4769d2b954/concepts/bf42ab6c-89db-4459-898f-4f270162edc0
     */
    
    //predicted state mean
    x_.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //iterate over sigma points
        x_ = x_ + weights_(i) * Xsig_pred_.col(i);
    }
    
    //predicted state covariance matrix
    P_.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //iterate over sigma points
        
        // state difference
        VectorXd x_diff = Xsig_pred_.col(i) - x_;
        //VectorXd x_diff = Xsig_pred_.col(i) - Xsig_pred_.col(0);
        //angle normalization
        /**
         When calculating the predicted state covariance matrix, I did something you might not have done. In the equation we always need the difference between the mean predicted state and a sigma points. The problem here is that the state contains an angle. As you have learned before, subtracting angles is a problem for Kalman filters, because the result might be 2π plus a small angle, instead of just a small angle. That’s why I normalize the angle here.
         
         Make sure you always normalize when you calculate the difference between angles.
         */
        
        while (x_diff(3)> M_PI) x_diff(3)-=2.*M_PI;
        while (x_diff(3)<-M_PI) x_diff(3)+=2.*M_PI;
        
        
        P_ = P_ + weights_(i) * x_diff * x_diff.transpose() ;
    }
    
    
}

/**
 * Updates the state and the state covariance matrix using a laser measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateLidar(MeasurementPackage meas_package) {
    /**
     TODO:
     
     Complete this function! Use lidar data to update the belief about the object's
     position. Modify the state vector, x_, and covariance, P_.
     
     You'll also need to calculate the lidar NIS.
     */
    //set measurement dimension, radar can measure r, phi, and r_dot
    //int n_z = 3;
    //set measurement dimension, lidar can measure px & py
    int n_z = 2;
    
    //create matrix for sigma points in measurement space
    MatrixXd Zsig = MatrixXd(n_z, 2 * n_aug_ + 1);
    
    //mean predicted measurement
    VectorXd z_pred = VectorXd(n_z);
    
    //measurement covariance matrix S
    MatrixXd S = MatrixXd(n_z,n_z);
    
    //std::cout << "IN LIDAR" << std::endl;
    
    /**
     classroom.udacity.com/nanodegrees/nd013/parts/40f38239-66b6-46ec-ae68-03afd8a601c8/modules/0949fca6-b379-42af-a919-ee50aa304e6a/lessons/daf3dee8-7117-48e8-a27a-fc4769d2b954/concepts/d1a4ce03-73aa-4653-b216-1c6d965fc216
     */
    
    //double weight_0 = lambda_/(lambda_+n_aug_);
    //weights(0) = weight_0;
    //for (int i=1; i<2*n_aug_+1; i++) {
    //  double weight = 0.5/(n_aug_+lambda_);
    //  weights(i) = weight;
    //}
    
    //transform sigma points into measurement space
    //calculate mean predicted measurement
    //calculate measurement covariance matrix S
    //transform sigma points into measurement space
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //2n+1 simga points
        
        // extract values for better readibility
        double p_x = Xsig_pred_(0,i);
        double p_y = Xsig_pred_(1,i);
        double v  = Xsig_pred_(2,i);
        double yaw = Xsig_pred_(3,i);
        
        double v1 = cos(yaw)*v;
        double v2 = sin(yaw)*v;
        
        // measurement model - LIDAR
        // instead of:----------------------
        // initializing matrices
        //MatrixXd H_laser_ = MatrixXd(2, 4);
        //H_laser_ << 1, 0, 0, 0, 0,
        //            0, 1, 0, 0, 0;
        // DO:------------------------------
        // n_z = 2
        Zsig(0,i) = p_x;                        //px
        Zsig(1,i) = p_y;                        //py
        
    }
    
    //mean predicted measurement
    //VectorXd z_pred = VectorXd(n_z);
    z_pred.fill(0.0);
    for (int i=0; i < 2*n_aug_+1; i++) {
        z_pred = z_pred + weights_(i) * Zsig.col(i);
    }
    
    //measurement covariance matrix S
    //MatrixXd S = MatrixXd(n_z,n_z);
    S.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //2n+1 simga points
        //residual
        VectorXd z_diff = Zsig.col(i) - z_pred;
        
        // NO Normalization can be done here b/c NOT angles
        //angle normalization
        //while (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
        //while (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;
        //z_diff(1) = atan2(sin(z_diff(1)),cos(z_diff(1)));  // Normalize phi=z_diff(1) to be between -pi and +pi
        //std::cout << "z_diff(1) = " << z_diff(1) << std::endl;
        
        S = S + weights_(i) * z_diff * z_diff.transpose();
    }
    
    //add measurement noise covariance matrix
    
    //measurement covariance matrix - laser
    // R = R_laser_
    MatrixXd R = MatrixXd(n_z,n_z);
    R << std_laspx_*std_laspx_, 0,
    0, std_laspy_*std_laspy_;
    
    S = S + R;
    
    /**
     classroom.udacity.com/nanodegrees/nd013/parts/40f38239-66b6-46ec-ae68-03afd8a601c8/modules/0949fca6-b379-42af-a919-ee50aa304e6a/lessons/daf3dee8-7117-48e8-a27a-fc4769d2b954/concepts/e19bc36c-a671-4799-8c63-cec40544c2aa
     */
    
    //create matrix for cross correlation
    MatrixXd Tc = MatrixXd(n_x_, n_z);
    
    Tc.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //2n+1 simga points
        
        //residual
        VectorXd z_diff = Zsig.col(i) - z_pred;
        // NO Normalization can be done here b/c NOT angles
        //angle normalization
        
        
        // state difference
        VectorXd x_diff = Xsig_pred_.col(i) - x_;
        //VectorXd x_diff = Xsig_pred_.col(i) - Xsig_pred_.col(0);
        //angle normalization
        
        while (x_diff(3)> M_PI) x_diff(3)-=2.*M_PI;
        while (x_diff(3)<-M_PI) x_diff(3)+=2.*M_PI;
        //x_diff(3) = atan2(sin(x_diff(3)),cos(x_diff(3)));  // Normalize phi=x_diff(3) to be between -pi and +pi
        
        
        Tc = Tc + weights_(i) * x_diff * z_diff.transpose();
    }
    
    //Kalman gain K;
    MatrixXd K = Tc * S.inverse();
    
    //z = [px py]' = actual measurement - here and in NIS
    VectorXd z = meas_package.raw_measurements_;
    
    //residual
    VectorXd z_diff = z - z_pred;
    
    // NO Normalization can be done here b/c NOT angles
    //angle normalization
    
    
    //update state mean and covariance matrix
    x_ = x_ + K * z_diff;
    P_ = P_ - K*S*K.transpose();
    
    //^^^^^^^^^^^^^^ NIS ^^^^^^^^^^^^^^^^
    double epsilon = (z - z_pred).transpose()*S.inverse()*(z - z_pred);
   
    
    
}

/**
 * Updates the state and the state covariance matrix using a radar measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateRadar(MeasurementPackage meas_package) {
    /**
     TODO:
     
     Complete this function! Use radar data to update the belief about the object's
     position. Modify the state vector, x_, and covariance, P_.
     
     You'll also need to calculate the radar NIS.
     */
    //create vector for weights
    //VectorXd weights = VectorXd(2*n_aug_+1);
    
    //set measurement dimension, radar can measure r, phi, and r_dot
    int n_z = 3;
    //set measurement dimension, lidar can measure px & py
    //int n_z = 2;
    
    //create matrix for sigma points in measurement space
    MatrixXd Zsig = MatrixXd(n_z, 2 * n_aug_ + 1);
    
    //mean predicted measurement
    VectorXd z_pred = VectorXd(n_z);
    
    //measurement covariance matrix S
    MatrixXd S = MatrixXd(n_z,n_z);
    
    /**
     classroom.udacity.com/nanodegrees/nd013/parts/40f38239-66b6-46ec-ae68-03afd8a601c8/modules/0949fca6-b379-42af-a919-ee50aa304e6a/lessons/daf3dee8-7117-48e8-a27a-fc4769d2b954/concepts/d1a4ce03-73aa-4653-b216-1c6d965fc216
     */
    
    //transform sigma points into measurement space
    //calculate mean predicted measurement
    //calculate measurement covariance matrix S
    //transform sigma points into measurement space
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //2n+1 simga points
        
        // extract values for better readibility
        double p_x = Xsig_pred_(0,i);
        double p_y = Xsig_pred_(1,i);
        double v  = Xsig_pred_(2,i);
        double yaw = Xsig_pred_(3,i);
        
        double v1 = cos(yaw)*v;
        double v2 = sin(yaw)*v;
        
        // measurement model - RADAR
        //Zsig(0,i) = sqrt(p_x*p_x + p_y*p_y);                        //r
        //Zsig(1,i) = atan2(p_y,p_x);                                 //phi
        //Zsig(2,i) = (p_x*v1 + p_y*v2 ) / sqrt(p_x*p_x + p_y*p_y);   //r_dot
        
        if (p_x == 0 && p_y == 0) {
            Zsig(0,i) = 0;
            Zsig(1,i) = 0;
            Zsig(2,i) = 0;
        } else {
            Zsig(0,i) = sqrt(p_x*p_x + p_y*p_y);
            Zsig(1,i) = atan2(p_y, p_x);
            Zsig(2,i) = (p_x*v1 + p_y*v2)/sqrt(p_x*p_x + p_y*p_y);
        }
    }
    
    //mean predicted measurement
    //VectorXd z_pred = VectorXd(n_z);
    z_pred.fill(0.0);
    for (int i=0; i < 2*n_aug_+1; i++) {
        z_pred = z_pred + weights_(i) * Zsig.col(i);
    }
    
    //measurement covariance matrix S
    //MatrixXd S = MatrixXd(n_z,n_z);
    S.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //2n+1 simga points
        //residual
        VectorXd z_diff = Zsig.col(i) - z_pred;
        
        //angle normalization
        //std::cout << "Before1 RADAR z_diff(1) = " << z_diff(1) << std::endl;
        while (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
        while (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;
        
        
        S = S + weights_(i) * z_diff * z_diff.transpose();
    }
    
    //add measurement noise covariance matrix
    
    
    
    //measurement covariance matrix - radar
    // R = R_radar_
    MatrixXd R = MatrixXd(n_z,n_z);
    R <<    std_radr_*std_radr_, 0, 0,
    0, std_radphi_*std_radphi_, 0,
    0, 0,std_radrd_*std_radrd_;
    
    S = S + R;
    
    /**
     classroom.udacity.com/nanodegrees/nd013/parts/40f38239-66b6-46ec-ae68-03afd8a601c8/modules/0949fca6-b379-42af-a919-ee50aa304e6a/lessons/daf3dee8-7117-48e8-a27a-fc4769d2b954/concepts/e19bc36c-a671-4799-8c63-cec40544c2aa
     */
    
    //create matrix for cross correlation
    MatrixXd Tc = MatrixXd(n_x_, n_z);
    
    Tc.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {  //2n+1 simga points
        
        //residual
        VectorXd z_diff = Zsig.col(i) - z_pred;
        //angle normalization
        //std::cout << "Before2 RADAR z_diff(1) = " << z_diff(1) << std::endl;
        while (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
        while (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;
        //z_diff(1) = atan2(sin(z_diff(1)),cos(z_diff(1)));  // Normalize phi=z_diff(1) to be between -pi and +pi
        //std::cout << "After2 RADAR z_diff(1) = " << z_diff(1) << std::endl;
        
        // state difference
        VectorXd x_diff = Xsig_pred_.col(i) - x_;
        //VectorXd x_diff = Xsig_pred_.col(i) - Xsig_pred_.col(0);
        //angle normalization
        //std::cout << "Before RADAR x_diff(3) = " << x_diff(3) << std::endl;
        while (x_diff(3)> M_PI) x_diff(3)-=2.*M_PI;
        while (x_diff(3)<-M_PI) x_diff(3)+=2.*M_PI;
        //x_diff(3) = atan2(sin(x_diff(3)),cos(x_diff(3)));  // Normalize phi=x_diff(3) to be between -pi and +pi
        //std::cout << "After RADAR x_diff(3) = " << x_diff(3) << std::endl;
        
        Tc = Tc + weights_(i) * x_diff * z_diff.transpose();
    }
    
    //Kalman gain K;
    MatrixXd K = Tc * S.inverse();
    
    // z = [rho phi rhodot]' = actual measurement - here and in NIS
    VectorXd z = meas_package.raw_measurements_;
    
    //residual
    VectorXd z_diff = z - z_pred;
    
    //angle normalization
    //std::cout << "Before3 RADAR z_diff(1) = " << z_diff(1) << std::endl;
    while (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
    while (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;
    
    
    //update state mean and covariance matrix
    x_ = x_ + K * z_diff;
    P_ = P_ - K*S*K.transpose();
    
    
    //^^^^^^^^^^^^^^ NIS ^^^^^^^^^^^^^^^^
    double epsilon = (z - z_pred).transpose()*S.inverse()*(z - z_pred);
    
}
