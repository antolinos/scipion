/***************************************************************************
 *
 * Authors:     Carlos Oscar S. Sorzano (coss@cnb.uam.es)
 *
 * Unidad de  Bioinformatica of Centro Nacional de Biotecnologia , CSIC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.uam.es'
 ***************************************************************************/

#include "art_crystal.h"
#include "project_crystal.h"

/* ------------------------------------------------------------------------- */
/* Crystal ART Parameters                                                    */
/* ------------------------------------------------------------------------- */
/* Read ART parameters ===================================================== */
void Crystal_ART_Parameters::read(int argc, char **argv,
                                  Basic_ART_Parameters &prm)
{
    try
    {
        prm.is_crystal = true;
        a_mag = textToFloat(getParameter(argc, argv, "-mag_a"));
        a_mag /= prm.sampling;
        b_mag = textToFloat(getParameter(argc, argv, "-mag_b"));
        b_mag /= prm.sampling;
        ang_a2b_deg = textToFloat(getParameter(argc, argv, "-ang_a2b_deg"));
        ang_x2a_deg = textToFloat(getParameter(argc, argv, "-ang_x2a_deg", 0));
        fill_space = checkParameter(argc, argv, "-fill_space");
        avox.resize(2);
        bvox.resize(2);
        //DISCLAMER: I know this 0 and 90 degrees cases are rather silly but
        //when debuging is so good that 90 is 90 and not 90.000001
        //NOTE:ang_x2a_deg is applied ONLY in the final volume
        //when moving from basis to voxels

        XX(avox) = a_mag;
        YY(avox) = 0.;

        if (ang_a2b_deg == 90.)
        {
            XX(bvox) = 0;
            YY(bvox) = b_mag;
        }
        else
        {
            XX(bvox) = b_mag * COSD(ang_a2b_deg);
            YY(bvox) = b_mag * SIND(ang_a2b_deg);
        }
    }
    catch (Xmipp_error XE)
    {
        throw(XE);
    }

}

/* Usage =================================================================== */
void Crystal_ART_Parameters::usage_more()
{
    printf(
        "Special Parameters for crystals -----------------------------------\n"
        "    -crystal:            Crystal mode activation\n"
        "Lattice parameters:\n"
        "    -mag_a mag   Magnitude of the first  lattice vector\n"
        "    -mag_b mag   Magnitude of the second lattice vector\n"
        "    -ang_a2b_deg angle    angle from vector a to vector b\n"
        "    -ang_x2a_deg angle    angle from vector x to vector a\n"
        "   [-fill_space]         Repeat unit cell all over the space (pixels)\n"
    );
}

/* Show parameters ========================================================= */
std::ostream & operator << (std::ostream &o, const Crystal_ART_Parameters &eprm)
{
    o << "Lattice vector a: " << eprm.a.transpose() << std::endl;
    o << "Lattice vector b: " << eprm.b.transpose() << std::endl;
    o << "mag_a: " << eprm.a_mag << std::endl;
    o << "mag_b: " << eprm.b_mag << std::endl;
    o << "ang_a2b_deg: " << eprm.ang_a2b_deg << std::endl;
    o << "ang_x2a_deg: " << eprm.ang_x2a_deg << std::endl;
    o << "Fill space: " << eprm.fill_space << std::endl;
    o << "Symmetry group: " <<  eprm.space_group;
    return o;
}

/* Compute integer lattice ================================================= */
/* Compute integer lattice ================================================= */
void compute_integer_lattice(const Matrix1D<double> &a,
                             const Matrix1D<double> &b,
                             double a_mag_grid, double b_mag_grid,
                             double ang_a2b_deg,
                             Matrix1D<double> &aint,
                             Matrix1D<double> &bint,
                             Matrix2D<double> &D,
                             int space_group)
{

    // Integer lattice
    // If we force it to be orthogonal the symmetrization is easier
    //a_mag=original a_mag/sampling/grid_relatice_size
    aint.resize(2);
    bint.resize(2);
    XX(aint) = ROUND(a_mag_grid);
    YY(aint) = 0.;
    XX(bint) = 0.0;
    YY(bint) = ROUND(b_mag_grid);

    //different crystalline grids impose different restrictions
    //on the grid, we will check them here but only if the user
    //has provided a symmetry file

    if (space_group != sym_undefined)
    {

        switch (space_group)
        {

        case sym_P1:
            break;// no check needed
        case sym_P2_122://XX(aint) and YY(aint) should be even
            if (XX(aint) != 2*(int)(XX(aint) / 2) ||
                YY(bint) != 2*(int)(YY(bint) / 2))
            {
                std::cout << "\nLattice connstrains for P2_122 are not satisficed"
                << "\nRound[mag_a/(sampling*grid_size)] must be even"
                << "\nPlease modify the parmeters and try again" << std::endl;
                exit(0);
            }
            break;
        case sym_P22_12://XX(aint) and YY(aint) should be even
            if (XX(aint) != 2*(int)(XX(aint) / 2) ||
                YY(bint) != 2*(int)(YY(bint) / 2))
            {
                std::cout << "\nLattice connstrains for P22_12 are not satisficed"
                << "\nRound[mag_a/(sampling*grid_size)] must be even"
                << "\nPlease modify the parmeters and try again" << std::endl;
                exit(0);
            }
            break;
        case sym_P42_12://XX(aint) and YY(aint) should be even
            if (XX(aint) != 2*(int)(XX(aint) / 2) ||
                YY(bint) != 2*(int)(YY(bint) / 2))
            {
                std::cout << "\nLattice connstrains for P4212 are not satisficed"
                << "\nRound[mag_a/(sampling*grid_size)] must be even"
                << "\nPlease modify the parmeters and try again" << std::endl;
                exit(0);
            }
            break;
        case sym_P4:
        case sym_P6:
            break;// no check needed
        default:
            std::cerr << "\n Congratulations: you have found a bug in the\n"
            << "routine compute_integer_lattice or\n"
            << "you are using a non implemented symmetry group\n"
            <<  std::endl;
            exit(0);
            break;
        }//switch(space_group)  end

    }//if (prm.fn_sym!="")


    // Converting matrix
    // a=D*aint
    // b=D*bint
    Matrix2D<double> L(2, 2), LI(2, 2);

    L .setCol(0, a);
    L .setCol(1, b);
    LI.setCol(0, aint);
    LI.setCol(1, bint);
    D = L * LI.inv();
}

/* Special vector product ================================================== */
/* To see if a point is inside a polygon the vector product of cir
   (the vector which goes from the point (r) to the corner i (ci)
   position) and each polygon side is computed. This vector product
   assume that both vectors are in the same plane (XY), so its z component
   is 0, and we are only interested on the sign of z in the resulting
   vector. */
#define SGN0_VEC_PRODUCT(cir,a) SGN0(XX(cir)*YY(a)-YY(cir)*XX(a))

/* Side information ======================================================== */
//#define DEBUG
//#define DEBUG_A_LOT
void Crystal_ART_Parameters::produce_Side_Info(
    Basic_ART_Parameters &prm, GridVolume &vol_basis0)
{

    // Lattice vectors in BCC units
    a = avox / prm.grid_relative_size;
    b = bvox / prm.grid_relative_size;

    // Compute space_group
    if (prm.fn_sym != "")
        space_group = prm.SL.crystallographic_space_group(a_mag, b_mag,
                      ang_a2b_deg);
    else
        space_group = sym_P1;

    // Integer lattice vectors ----------------------------------------------
    Matrix2D<double> D;
    compute_integer_lattice(a, b, a_mag / prm.grid_relative_size,
                            b_mag / prm.grid_relative_size,
                            ang_a2b_deg, aint, bint, D,
                            space_group);
    // Define two more useful lattice vectors
    ai = aint / 2;
    bi = bint / 2;

    // Set general deformation pointer to this matrix
    prm.D = new Matrix2D<double>;
    *(prm.D) = D;
    prm.D->resize(3, 3);
    MAT_ELEM(*(prm.D), 2, 2) = 1;
    prm.Dinv = new Matrix2D<double>;
    *(prm.Dinv) = prm.D->inv();
    prm.basis.set_D(prm.D);

    // Unit cell mask within volume -----------------------------------------
    // Compute the 4 parallelogram corners
    Matrix1D<double> c1 = ai + bi;
    Matrix1D<double> c2 = -ai + bi;
    Matrix1D<double> c3 = -c1;
    Matrix1D<double> c4 = -c2;
    Matrix1D<double> c1c3 = c1 - c3; // These extra variables are needed because
    Matrix1D<double> c2c4 = c2 - c4; // the compiler messes up

    // Resize unit cell mask
    // The unit mask is a little bigger to avoid the possibility of losing
    // any basis due to a too tight mask.
    int mask_xdim = CEIL(XMIPP_MAX(ABS(XX(c1c3)), ABS(XX(c2c4)))) + 3;
    int mask_ydim = CEIL(XMIPP_MAX(ABS(YY(c1c3)), ABS(YY(c2c4)))) + 3;
    unit_cell_mask.initZeros(mask_ydim, mask_xdim);
    unit_cell_mask.setXmippOrigin();

    // Resize the reconstructed volume
    Matrix1D<double> r1(3), r2(3);
    for (int n = 0; n < vol_basis0.VolumesNo(); n++)
    {
        Volume &V = vol_basis0(n);
        ZZ(r1) = XMIPP_MIN(ZZ(r1), STARTINGZ(V()));
        ZZ(r2) = XMIPP_MAX(ZZ(r2), FINISHINGZ(V()));
    }
    XX(r1) = STARTINGX(unit_cell_mask);
    YY(r1) = STARTINGY(unit_cell_mask);
    r1 *= prm.grid_relative_size;
    XX(r2) = FINISHINGX(unit_cell_mask);
    YY(r2) = FINISHINGY(unit_cell_mask);
    r2 *= prm.grid_relative_size;
    vol_basis0.resize(r1, r2);

    // Fill the unit cell mask
    Matrix1D<double> r(2);
    FOR_ALL_ELEMENTS_IN_MATRIX2D(unit_cell_mask)
    {
        // Position vector of actual BCC element been considered
        YY(r) = i;
        XX(r) = j;

        // Vectors from r to each corner
        Matrix1D<double> c1r = c1 - r;
        Matrix1D<double> c2r = c2 - r;
        Matrix1D<double> c3r = c3 - r;
        Matrix1D<double> c4r = c4 - r;

        // Product of each of these vectors with tha parallelogram borders
        int sgn[4];
        sgn[0] = SGN0_VEC_PRODUCT(c1r, -ai);
        sgn[1] = SGN0_VEC_PRODUCT(c2r, -bi);
        sgn[2] = SGN0_VEC_PRODUCT(c3r, ai);
        sgn[3] = SGN0_VEC_PRODUCT(c4r, bi);

#ifdef DEBUG_A_LOT
        std::cout << "(x,y)=(" << XX(r) << "," << YY(r) << ") " << sgn[0] << sgn[1]
        << sgn[2] << sgn[3] << std::endl;
#endif

        // Now check if point is inside
        int inside_table[4][4] = {
                                     1, 1, 1, 1,
                                     0, 1, 1, 1,
                                     1, 0, 1, 1,
                                     // 1,1,0,1, Take this side out
                                     // 1,1,1,0, Take this side out
                                     0, 0, 1, 1,
                                     // 1,0,0,1, and their three vertices
                                     // 1,1,0,0,
                                     // 0,1,1,0
                                 };
#define INSIDE_ACCORDING_TO(n) \
    sgn[0]==inside_table[n][0] && sgn[1]==inside_table[n][1] && \
    sgn[2]==inside_table[n][2] && sgn[3]==inside_table[n][3]
        for (int n = 0; n < 4; n++)
            if (INSIDE_ACCORDING_TO(n))
            {
                unit_cell_mask(i, j) = 1;
#ifdef DEBUG_A_LOT
                std::cout << "    Inside\n";
#endif
                break;
            }
    }

    // Show all parameters --------------------------------------------------
#ifdef DEBUG
    std::cout << "avox= " << avox.transpose() << std::endl;
    std::cout << "bvox= " << bvox.transpose() << std::endl;
    std::cout << "grid_relative_size= " << prm.grid_relative_size << std::endl;
    std::cout << "a=    " << a.transpose()    << std::endl;
    std::cout << "b=    " << b.transpose()    << std::endl;
    std::cout << "aint= " << aint.transpose() << std::endl;
    std::cout << "bint= " << bint.transpose() << std::endl;
    std::cout << "ai=   " << ai.transpose()   << std::endl;
    std::cout << "bi=   " << bi.transpose()   << std::endl;
    std::cout << "D=    " << D;
    std::cout << "Check that a=D*aint " << (D*aint).transpose() << std::endl;
    std::cout << "Check that b=D*bint " << (D*bint).transpose() << std::endl;
    std::cout << "Symmetry group: " <<  space_group;
    ImageXmipp I;
    I = unit_cell_mask;
    I.write("unit_cell_mask.xmp");
    std::cout << "unit_cell_mask shape=";
    unit_cell_mask.printShape();
    std::cout << std::endl;
#endif
}
#undef DEBUG
#undef DEBUG_A_LOT

/* ------------------------------------------------------------------------- */
/* ART Single step                                                           */
/* ------------------------------------------------------------------------- */
void ART_single_step(
    GridVolume              &vol_in,          // Input Reconstructed volume
    GridVolume              *vol_out,         // Output Reconstructed volume
    Basic_ART_Parameters    &prm,             // blob, lambda
    Crystal_ART_Parameters  &eprm,            // lattice vectors, ...
    Projection              &theo_proj,       // Projection of the reconstruction
                                              // It is outside to make it visible
                                              // just if it needed for any
                                              // other purpose
    Projection              &read_proj,       // Real projection
    int sym_no,                               // Symmetry matrix index
    Projection              &diff_proj,       // Difference between read and
                                              // theoretical projection
    Projection              &corr_proj,       // Correcting projection
    Projection              &alig_proj,       // Translation alignement aux proj
    double                  &mean_error,      // Mean error over the pixels
    int                      numIMG,          // number of images in the set
                                              // in SIRT the correction must
                                              // be divided by this number
    double                   lambda,          // Lambda to be used
    int                      imagen_no,       // Projection number
    const FileName          &fn_ctf,          // CTF to apply
    const Matrix2D<int> *    maskPtr          // Mask to apply
    )           
{
// Only works for blob volumes .............................................
    if (prm.basis.type != Basis::blobs)
        REPORT_ERROR(1,
                     "ART_single_step: This function only works with blob volumes");

// Compute lattice vectors to be used ......................................
    Matrix1D<double> aint, bint, shift;
    aint.resize(2);
    bint.resize(2);
    shift.resize(3);
    shift.initZeros();

    symmetrize_crystal_vectors(aint, bint, shift, eprm.space_group, sym_no,
                               eprm.aint, eprm.bint);
// Project structure .......................................................
    // The correction image is reused in this call to store the normalising
    // projection, ie, the projection of an all-1 volume

    project_Crystal_Volume(vol_in, prm.basis, theo_proj,
                           corr_proj, YSIZE(read_proj()), XSIZE(read_proj()),
                           read_proj.rot(), read_proj.tilt(), read_proj.psi(), shift,
                           aint, bint, *(prm.D), *(prm.Dinv), eprm.unit_cell_mask,
                           FORWARD, prm.eq_mode);
    double shift_X, shift_Y;

//   #define DEBUG_SHIFT
#ifdef DEBUG_SHIFT
    Matrix2D<double> A(3, 3);
    A.initIdentity();
    dMij(A, 0, 2) =  8;
    dMij(A, 1, 2) =  -5;
    std::cout << "A" << A;
    //move read_proj
    FOR_ALL_DIRECT_ELEMENTS_IN_MATRIX2D(theo_proj())
    dMij(theo_proj(), i, j) = dMij(read_proj(), i, j);
    applyGeometry(IMGMATRIX(read_proj), A, IMGMATRIX(theo_proj), IS_NOT_INV, WRAP);
#endif
#undef DEBUG_SHIFT
    if (prm.ref_trans_after != -1    &&
        imagen_no > prm.ref_trans_after && imagen_no != 0)
    {
        calculate_and_find_correlation_max_proj(read_proj, theo_proj,
                                                alig_proj,
                                                shift_X, shift_Y,
                                                prm.ref_trans_step,
                                                prm.ref_trans_after,
                                                imagen_no);

        // Apply correction
        Matrix2D<double> Correction(3, 3);
        alig_proj().resize(read_proj());
        Correction.initIdentity();

        dMij(Correction, 0, 2) =  - shift_X;
        dMij(Correction, 1, 2) =  - shift_Y;
        //copy theo_proj to a temporal matrix
        FOR_ALL_DIRECT_ELEMENTS_IN_MATRIX2D(theo_proj())
        dMij(alig_proj(), i, j) = dMij(read_proj(), i, j);
        applyGeometry(IMGMATRIX(read_proj), Correction, IMGMATRIX(alig_proj), IS_NOT_INV, WRAP);
    }
// Now compute differences .................................................
    double applied_lambda = lambda / numIMG; // In ART mode, numIMG=1
    mean_error = 0;
    diff_proj().resize(read_proj());

    FOR_ALL_ELEMENTS_IN_MATRIX2D(IMGMATRIX(read_proj))
    {
        // Compute difference image and error
        IMGPIXEL(diff_proj, i, j) = IMGPIXEL(read_proj, i, j) - IMGPIXEL(theo_proj, i, j);
        mean_error += IMGPIXEL(diff_proj, i, j) * IMGPIXEL(diff_proj, i, j);

        // Compute the correction image
        if (ABS(IMGPIXEL(corr_proj, i, j)) < 1)
            IMGPIXEL(corr_proj, i, j) = SGN(IMGPIXEL(corr_proj, i, j));
        IMGPIXEL(corr_proj, i, j) =
            applied_lambda * IMGPIXEL(diff_proj, i, j) / IMGPIXEL(corr_proj, i, j);
    }
    mean_error /= XSIZE(diff_proj()) * YSIZE(diff_proj());

// Backprojection of correction plane ......................................
    project_Crystal_Volume(*vol_out, prm.basis, theo_proj,
                           corr_proj, YSIZE(read_proj()), XSIZE(read_proj()),
                           read_proj.rot(), read_proj.tilt(), read_proj.psi(), shift,
                           aint, bint, *(prm.D), *(prm.Dinv), eprm.unit_cell_mask,
                           BACKWARD, prm.eq_mode);
}

/* Force the {\it trial} volume to be symmetric.----------------------------*/
void apply_symmetry(GridVolume &vol_in, GridVolume *vol_out,
                    const Crystal_ART_Parameters &eprm, int grid_type)
{
    symmetrize_crystal_volume(vol_in, eprm.aint, eprm.bint,
                              eprm.space_group, eprm.unit_cell_mask,
                              grid_type);
}

/* Finish iterations ------------------------------------------------------- */
void finish_ART_iterations(const Basic_ART_Parameters &prm,
                           const Crystal_ART_Parameters &eprm, GridVolume &vol_basis)
{
    if (eprm.fill_space)
        expand_to_fill_space(prm, eprm, vol_basis);
}

/* Expansion to fill space ------------------------------------------------- */
//#define DEBUG
//#define DEBUG2
void expand_to_fill_space(const Basic_ART_Parameters &prm,
                          const Crystal_ART_Parameters &eprm, GridVolume &vol)
{
    std::cerr << "Replicating unit cell ...\n";
#ifdef DEBUG
    ImageXmipp save;
    save = vol(0)().getSlice(0);
    save.write("inter_before_filling.xmp");
#endif

    // Resize volume ........................................................
    Matrix1D<double> corner1(2), corner2(2);
    VECTOR_R2(corner1,
              FIRST_XMIPP_INDEX(prm.Xoutput_volume_size),
              FIRST_XMIPP_INDEX(prm.Youtput_volume_size));
    VECTOR_R2(corner2,
              LAST_XMIPP_INDEX(prm.Xoutput_volume_size),
              LAST_XMIPP_INDEX(prm.Youtput_volume_size));

    Matrix1D<double> zero(2);
    VECTOR_R2(zero, 0, 0);
    int a0, aF, b0, bF;
    // How many lattice units fit inside the output volume
    find_crystal_limits(corner1, corner2, zero, zero,
                        eprm.a, eprm.b, a0, aF, b0, bF);
#ifdef DEBUG
    std::cout << "Output Volume size (ZxYxX)=" << prm.Zoutput_volume_size
    << " " << prm.Youtput_volume_size << " "
    << prm.Xoutput_volume_size << std::endl;
    std::cout << "corners:\n" << corner1.transpose() << std::endl
    << corner2.transpose() << std::endl;
    std::cout << "a0=" << a0 << "..." << aF << std::endl
    << "b0=" << b0 << "..." << bF << std::endl;
#endif

    // Expand the volume to the required space
    corner1 = (a0 - 1) * eprm.a + (b0 - 1) * eprm.b; // CO: I'm not very satisfied with
    corner2 = (aF + 1) * eprm.a + (bF + 1) * eprm.b; //     the +1 and -1 but it works
    corner1.resize(3);
    ZZ(corner1) = FIRST_XMIPP_INDEX(prm.Zoutput_volume_size);
    corner2.resize(3);
    ZZ(corner2) = LAST_XMIPP_INDEX(prm.Zoutput_volume_size);
    vol.resize(corner1, corner2);

    // Copy values ..........................................................
    Matrix1D<double> r(3);
    for (int n = 0; n < vol.VolumesNo(); n++)
    {
        Volume &V = vol(n);
        FOR_ALL_ELEMENTS_IN_MATRIX2D(eprm.unit_cell_mask)
        {
            if (MAT_ELEM(eprm.unit_cell_mask, i, j))
            {
#ifdef DEBUG2
                std::cout << "(y,x)=(" << i << "," << j << ") is inside\n";
#endif
                for (int ii = b0; ii <= bF; ii++)
                    for (int jj = a0; jj <= aF; jj++)
                    {
                        if (jj == 0 && ii == 0) continue;
                        XX(r) = j + ii * XX(eprm.bint) + jj * XX(eprm.aint);
                        YY(r) = i + ii * YY(eprm.bint) + jj * YY(eprm.aint);
                        ZZ(r) = STARTINGZ(VOLMATRIX(V));
                        if (!VOLMATRIX(V).outside(r))
                        {
#ifdef DEBUG2
                            std::cout << "   Is copied to (" << (int)YY(r) << ","
                            << (int)XX(r) << ")\n";
#endif
                            // copy values if inside volume
                            for (int k = STARTINGZ(VOLMATRIX(V));
                                 k <= FINISHINGZ(VOLMATRIX(V)); k++)
                                VOLVOXEL(V, k, (int)YY(r), (int)XX(r)) =
                                    VOLVOXEL(V, k, i, j);
                        }
                    }
            }
        }
    }
#ifdef DEBUG
    save = vol(0)().getSlice(0);
    save.write("inter_after_filling.xmp");
#endif
}
#undef DEBUG2
#undef DEBUG
