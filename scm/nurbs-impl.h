#ifndef NURBS_IMPL_H
#define NURBS_IMPL_H

typedef struct Point3D {
    double x, y, z;
} Point3D;

typedef struct NURBSCurve {
    int      degree;
    Point3D *control_points;
    double  *weights;
    int      n_knots;
} NURBSCurve;

/* Intersection "approximee" entre deux NURBS.
 * NOTE: la struct ne contient ni le nombre de points de controle ni le
 * vecteur de noeuds -> on ne peut pas faire une vraie evaluation De Boor.
 * On utilise donc la relation standard n_ctrl = n_knots - degree - 1,
 * et on approxime chaque courbe par son polygone de controle. La fonction
 * renvoie le milieu du couple de segments les plus proches entre les deux
 * polygones. A remplacer par un vrai solveur si tu as besoin de precision.
 */
Point3D Intersection(NURBSCurve *c1, NURBSCurve *c2);

/* Wrapper pour la FFI: chibi-ffi ne sait pas retourner une struct par
 * valeur, seulement par pointeur. Intersection() elle-meme garde ta
 * signature d'origine ; c'est ce wrapper qui est expose a Scheme. */
Point3D *Intersection_ffi(NURBSCurve *c1, NURBSCurve *c2);

/* Helper de test: construit une courbe "ligne" y = y0 + slope*i,
 * avec n_ctrl points de controle, respectant n_knots = n_ctrl+degree+1. */
NURBSCurve *make_line_curve(int degree, int n_ctrl, int n_knots,
                             double y0, double slope);

#endif
