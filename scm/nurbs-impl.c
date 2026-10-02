#include "nurbs-impl.h"
#include <stdlib.h>

static int num_control_points(const NURBSCurve *c) {
    /* relation standard: n_knots = n_control_points + degree + 1 */
    return c->n_knots - c->degree - 1;
}

static double clampd(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* points les plus proches entre segment (a0,a1) et segment (b0,b1) */
static void closest_segment_points(Point3D a0, Point3D a1, Point3D b0, Point3D b1,
                                    Point3D *pa, Point3D *pb) {
    double ux = a1.x - a0.x, uy = a1.y - a0.y, uz = a1.z - a0.z;
    double vx = b1.x - b0.x, vy = b1.y - b0.y, vz = b1.z - b0.z;
    double wx = a0.x - b0.x, wy = a0.y - b0.y, wz = a0.z - b0.z;

    double A = ux*ux + uy*uy + uz*uz;
    double B = ux*vx + uy*vy + uz*vz;
    double C = vx*vx + vy*vy + vz*vz;
    double D = ux*wx + uy*wy + uz*wz;
    double E = vx*wx + vy*wy + vz*wz;
    double denom = A*C - B*B;

    double s = (denom > 1e-12) ? clampd((B*E - C*D) / denom, 0.0, 1.0) : 0.0;
    double t = clampd((B*s + E) / (C > 1e-12 ? C : 1.0), 0.0, 1.0);
    s = clampd((B*t - D) / (A > 1e-12 ? A : 1.0), 0.0, 1.0);

    pa->x = a0.x + s*ux; pa->y = a0.y + s*uy; pa->z = a0.z + s*uz;
    pb->x = b0.x + t*vx; pb->y = b0.y + t*vy; pb->z = b0.z + t*vz;
}

static double dist2(Point3D a, Point3D b) {
    double dx = a.x-b.x, dy = a.y-b.y, dz = a.z-b.z;
    return dx*dx + dy*dy + dz*dz;
}

Point3D Intersection(NURBSCurve *c1, NURBSCurve *c2) {
    Point3D best_a = {0,0,0}, best_b = {0,0,0};
    double best_d = -1.0;

    int n1 = num_control_points(c1);
    int n2 = num_control_points(c2);

    for (int i = 0; i + 1 < n1; i++) {
        for (int j = 0; j + 1 < n2; j++) {
            Point3D pa, pb;
            closest_segment_points(c1->control_points[i], c1->control_points[i+1],
                                    c2->control_points[j], c2->control_points[j+1],
                                    &pa, &pb);
            double d = dist2(pa, pb);
            if (best_d < 0.0 || d < best_d) {
                best_d = d;
                best_a = pa;
                best_b = pb;
            }
        }
    }

    Point3D mid;
    mid.x = (best_a.x + best_b.x) / 2.0;
    mid.y = (best_a.y + best_b.y) / 2.0;
    mid.z = (best_a.z + best_b.z) / 2.0;
    return mid;
}

Point3D *Intersection_ffi(NURBSCurve *c1, NURBSCurve *c2) {
    Point3D *p = malloc(sizeof(Point3D));
    *p = Intersection(c1, c2);
    return p;
}

NURBSCurve *make_line_curve(int degree, int n_ctrl, int n_knots,
                             double y0, double slope) {
    NURBSCurve *c = malloc(sizeof(NURBSCurve));
    c->degree = degree;
    c->n_knots = n_knots;
    c->control_points = malloc(sizeof(Point3D) * n_ctrl);
    c->weights = malloc(sizeof(double) * n_ctrl);
    for (int i = 0; i < n_ctrl; i++) {
        c->control_points[i].x = (double)i;
        c->control_points[i].y = y0 + slope * (double)i;
        c->control_points[i].z = 0.0;
        c->weights[i] = 1.0;
    }
    return c;
}
