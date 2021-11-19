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
**             Date: November 11th, 2021
********************************************************************************/
#include <iostream>
#include "analysis/wavefront.h"
#include "sequential/sequential_trace.h"
#include "sequential/trace_error.h"

using namespace geopter;

Wavefront::Wavefront(OpticalSystem *opt_sys) :
    WaveAberration(opt_sys)
{

}

std::shared_ptr<MapData3d> Wavefront::plot(const Field* fld, double wvl, int nrd)
{
    const int num_wvls = opt_sys_->optical_spec()->spectral_region()->wvl_count();
    const double ref_wvl_val = opt_sys_->optical_spec()->spectral_region()->reference_wvl();
    const double nm_to_mm = 1.0e-6;
    const double convert_to_waves = 1.0/(nm_to_mm*ref_wvl_val);

    SequentialTrace *tracer = new SequentialTrace(opt_sys_);
    tracer->set_aperture_check(true);
    tracer->set_apply_vig(false);

    SequentialPath seq_path = tracer->sequential_path(wvl);

    auto chief_ray = tracer->trace_pupil_ray(Eigen::Vector2d({0.0, 0.0}), fld, wvl);

    std::vector<double> px;
    std::vector<double> py;

    double step = 2.0/(double)(nrd-1);
    for(int i = 0; i < nrd; i++){
        px.push_back( -1.0 + step*(double)i );
        py.push_back( -1.0 + step*(double)i );
    }

    GridArray<double> opd_grid(nrd, nrd);

    Eigen::Vector2d pupil;

    RayPtr ray = std::make_shared<Ray>(seq_path.size());

    for(int i = 0; i < nrd; i++)
    {
        for(int j = 0 ; j < nrd; j++)
        {          
            pupil(0) = -1.0 + step*(double)j;
            pupil(1) = -1.0 + step*(double)i;

            if(pupil.norm() < 1.0){
                ray->set_status(RayStatus::PassThrough);
                tracer->trace_pupil_ray(ray, seq_path, pupil, fld, wvl);

                if(ray->status() == RayStatus::PassThrough){
                    double opd = wave_abr_full_calc(ray, chief_ray);
                    opd *= convert_to_waves;
                    opd_grid.set_cell(i, j, opd);
                }else{
                    opd_grid.set_cell(i, j, NAN);
                }
            }else{
                opd_grid.set_cell(i, j, NAN);
            }

        }

    }

    auto mapdata = std::make_shared<MapData3d>();
    mapdata->set_data(px, py, opd_grid);

    delete tracer;

    return mapdata;
}
