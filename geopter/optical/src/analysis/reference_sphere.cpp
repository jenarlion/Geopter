#include "analysis/reference_sphere.h"

using namespace geopter;

ReferenceSphere::ReferenceSphere()
{
    ref_pt_  = Eigen::Vector3d::Zero(3);
    ref_dir_ = Eigen::Vector3d::Zero(3);
    radius_  = 0.0;
    exp_dist_parax_ = 0.0;
}

ReferenceSphere::ReferenceSphere(const Eigen::Vector3d& ref_pt, const Eigen::Vector3d& ref_dir, double radius, double exp_dist_parax)
{
    ref_pt_  = ref_pt;
    ref_dir_ = ref_dir;
    radius_  = radius;
    exp_dist_parax_ = exp_dist_parax;
}

Eigen::Vector3d ReferenceSphere::compute_ray_segment(RayPtr ray)
{
    Eigen::Vector3d cr_inc_pt_before_img;
    Eigen::Vector3d cr_dir_before_img;

    double h = cr_inc_pt_before_img(1);
    double u = cr_dir_before_img(1);
    constexpr double eps = 1.0e-14;

    double cr_exp_dist = 0.0;
    if(fabs(u) < eps){
        cr_exp_dist = exp_dist_parax_;
    }else{
        cr_exp_dist  = -h/u;
    }

    Eigen::Vector3d cr_exp_pt = cr_inc_pt_before_img + cr_exp_dist*cr_dir_before_img;
}
