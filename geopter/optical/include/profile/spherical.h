/*******************************************************************************
** Geopter
** Copyright (C) 2021 Hiiragi
** 
** This file is part of Geopter.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
** 
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
** 
** You should have received a copy of the GNU Lesser General Public
** License along with this library; If not, see <http://www.gnu.org/licenses/>.
********************************************************************************
**           Author: Hiiragi                                   
**          Website: https://github.com/heterophyllus/Geopter
**          Contact: heterophyllus.work@gmail.com                          
**             Date: May 16th, 2021                                                                                          
********************************************************************************/


#ifndef SPHERICAL_H
#define SPHERICAL_H

#include "surface_profile.h"

namespace geopter {

/** Standard spherical shape */
class Spherical
{
public:
    Spherical(){
        cv_ = 0.0;
    }
    Spherical(double c){
        cv_ = c;
    }

    std::string name() const{
        return "SPH";
    }

    double f(const Eigen::Vector3d& p) const {
        return p(2) - 0.5*cv_*(p.dot(p));
    }

    Eigen::Vector3d df(const Eigen::Vector3d& p) const{
        Eigen::Vector3d df({-cv_*p(0), -cv_*p(1), 1.0 - cv_*p(2)} );
        return df;
    }

    double sag(double x, double y) const;

    bool intersect(Eigen::Vector3d& pt, double& distance, const Eigen::Vector3d& p0, const Eigen::Vector3d& dir);

    void print(std::ostringstream& oss){};

protected:
    double cv_;
};




} //namespace geopter

#endif // SPHERICAL_H
