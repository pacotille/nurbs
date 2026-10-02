#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define MAX_INTERSECTIONS 100
#define TOLERANCE 1e-6
#define MAX_ITERATIONS 50

// Point structure
typedef struct {
    double x, y, z;
} Point3D;

// NURBS curve structure
typedef struct {
    int degree;
    int n_control_points;
    Point3D *control_points;
    double *weights;
    double *knots;
    int n_knots;
} NURBSCurve;

// Intersection result
typedef struct {
    Point3D point;
    double t1;  // Parameter on curve 1
    double t2;  // Parameter on curve 2
} Intersection;

// Helper functions
double distance(Point3D p1, Point3D p2) {
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    double dz = p1.z - p2.z;
    return sqrt(dx*dx + dy*dy + dz*dz);
}

// Find knot span for parameter t
int find_span(int n, int p, double t, double *knots) {
    if (t >= knots[n]) return n - 1;
    if (t <= knots[p]) return p;
    
    int low = p;
    int high = n;
    int mid = (low + high) / 2;
    
    while (t < knots[mid] || t >= knots[mid + 1]) {
        if (t < knots[mid]) high = mid;
        else low = mid;
        mid = (low + high) / 2;
    }
    return mid;
}

// Basis function calculation
void basis_funs(int i, double t, int p, double *knots, double *N) {
    double *left = (double*)malloc((p + 1) * sizeof(double));
    double *right = (double*)malloc((p + 1) * sizeof(double));
    
    N[0] = 1.0;
    for (int j = 1; j <= p; j++) {
        left[j] = t - knots[i + 1 - j];
        right[j] = knots[i + j] - t;
        double saved = 0.0;
        
        for (int r = 0; r < j; r++) {
            double temp = N[r] / (right[r + 1] + left[j - r]);
            N[r] = saved + right[r + 1] * temp;
            saved = left[j - r] * temp;
        }
        N[j] = saved;
    }
    
    free(left);
    free(right);
}

// Evaluate NURBS curve at parameter t
Point3D evaluate_nurbs(NURBSCurve *curve, double t) {
    int span = find_span(curve->n_control_points - 1, curve->degree, t, curve->knots);
    double *N = (double*)malloc((curve->degree + 1) * sizeof(double));
    basis_funs(span, t, curve->degree, curve->knots, N);
    
    Point3D point = {0.0, 0.0, 0.0};
    double w_sum = 0.0;
    
    for (int i = 0; i <= curve->degree; i++) {
        int idx = span - curve->degree + i;
        double nw = N[i] * curve->weights[idx];
        point.x += nw * curve->control_points[idx].x;
        point.y += nw * curve->control_points[idx].y;
        point.z += nw * curve->control_points[idx].z;
        w_sum += nw;
    }
    
    point.x /= w_sum;
    point.y /= w_sum;
    point.z /= w_sum;
    
    free(N);
    return point;
}

// Compute bounding box for curve segment
void compute_bbox(NURBSCurve *curve, double t_min, double t_max, 
                  Point3D *min, Point3D *max) {
    int n_samples = 10;
    min->x = min->y = min->z = INFINITY;
    max->x = max->y = max->z = -INFINITY;
    
    for (int i = 0; i <= n_samples; i++) {
        double t = t_min + (t_max - t_min) * i / n_samples;
        Point3D p = evaluate_nurbs(curve, t);
        
        if (p.x < min->x) min->x = p.x;
        if (p.y < min->y) min->y = p.y;
        if (p.z < min->z) min->z = p.z;
        if (p.x > max->x) max->x = p.x;
        if (p.y > max->y) max->y = p.y;
        if (p.z > max->z) max->z = p.z;
    }
}

// Check if two bounding boxes overlap
bool bbox_overlap(Point3D min1, Point3D max1, Point3D min2, Point3D max2) {
    return (min1.x <= max2.x && max1.x >= min2.x &&
            min1.y <= max2.y && max1.y >= min2.y &&
            min1.z <= max2.z && max1.z >= min2.z);
}

// Recursive subdivision intersection finder
void find_intersections_recursive(NURBSCurve *c1, double t1_min, double t1_max,
                                  NURBSCurve *c2, double t2_min, double t2_max,
                                  Intersection *results, int *count, int depth) {
    if (depth > MAX_ITERATIONS || *count >= MAX_INTERSECTIONS) return;
    
    // Compute bounding boxes
    Point3D min1, max1, min2, max2;
    compute_bbox(c1, t1_min, t1_max, &min1, &max1);
    compute_bbox(c2, t2_min, t2_max, &min2, &max2);
    
    // Check if bounding boxes overlap
    if (!bbox_overlap(min1, max1, min2, max2)) return;
    
    // Check if segments are small enough
    double dt1 = t1_max - t1_min;
    double dt2 = t2_max - t2_min;
    
    if (dt1 < TOLERANCE && dt2 < TOLERANCE) {
        // Found an intersection
        double t1_mid = (t1_min + t1_max) / 2.0;
        double t2_mid = (t2_min + t2_max) / 2.0;
        Point3D p1 = evaluate_nurbs(c1, t1_mid);
        Point3D p2 = evaluate_nurbs(c2, t2_mid);
        
        if (distance(p1, p2) < TOLERANCE * 10) {
            results[*count].point.x = (p1.x + p2.x) / 2.0;
            results[*count].point.y = (p1.y + p2.y) / 2.0;
            results[*count].point.z = (p1.z + p2.z) / 2.0;
            results[*count].t1 = t1_mid;
            results[*count].t2 = t2_mid;
            (*count)++;
        }
        return;
    }
    
    // Subdivide and recurse
    double t1_mid = (t1_min + t1_max) / 2.0;
    double t2_mid = (t2_min + t2_max) / 2.0;
    
    find_intersections_recursive(c1, t1_min, t1_mid, c2, t2_min, t2_mid, 
                                 results, count, depth + 1);
    find_intersections_recursive(c1, t1_min, t1_mid, c2, t2_mid, t2_max, 
                                 results, count, depth + 1);
    find_intersections_recursive(c1, t1_mid, t1_max, c2, t2_min, t2_mid, 
                                 results, count, depth + 1);
    find_intersections_recursive(c1, t1_mid, t1_max, c2, t2_mid, t2_max, 
                                 results, count, depth + 1);
}

// Main intersection function
int intersect_nurbs_curves(NURBSCurve *c1, NURBSCurve *c2, 
                           Intersection *results) {
    int count = 0;
    double t1_min = c1->knots[c1->degree];
    double t1_max = c1->knots[c1->n_control_points];
    double t2_min = c2->knots[c2->degree];
    double t2_max = c2->knots[c2->n_control_points];
    
    find_intersections_recursive(c1, t1_min, t1_max, c2, t2_min, t2_max, 
                                 results, &count, 0);
    return count;
}

// Example usage
int tt() {
    // Create first NURBS curve (quarter circle)
    NURBSCurve curve1;
    curve1.degree = 2;
    curve1.n_control_points = 3;
    curve1.control_points = (Point3D*)malloc(3 * sizeof(Point3D));
    curve1.weights = (double*)malloc(3 * sizeof(double));
    curve1.n_knots = 6;
    curve1.knots = (double*)malloc(6 * sizeof(double));
    
    curve1.control_points[0] = (Point3D){1.0, 0.0, 0.0};
    curve1.control_points[1] = (Point3D){1.0, 1.0, 0.0};
    curve1.control_points[2] = (Point3D){0.0, 1.0, 0.0};
    curve1.weights[0] = 1.0;
    curve1.weights[1] = 0.707;
    curve1.weights[2] = 1.0;
    curve1.knots[0] = curve1.knots[1] = curve1.knots[2] = 0.0;
    curve1.knots[3] = curve1.knots[4] = curve1.knots[5] = 1.0;
    
    // Create second NURBS curve (diagonal line)
    NURBSCurve curve2;
    curve2.degree = 1;
    curve2.n_control_points = 2;
    curve2.control_points = (Point3D*)malloc(2 * sizeof(Point3D));
    curve2.weights = (double*)malloc(2 * sizeof(double));
    curve2.n_knots = 4;
    curve2.knots = (double*)malloc(4 * sizeof(double));
    
    curve2.control_points[0] = (Point3D){0.0, 0.0, 0.0};
    curve2.control_points[1] = (Point3D){1.0, 1.0, 0.0};
    curve2.weights[0] = 1.0;
    curve2.weights[1] = 1.0;
    curve2.knots[0] = curve2.knots[1] = 0.0;
    curve2.knots[2] = curve2.knots[3] = 1.0;
    
    // Find intersections
    Intersection results[MAX_INTERSECTIONS];
    int n = intersect_nurbs_curves(&curve1, &curve2, results);
    
    printf("Found %d intersection(s):\n", n);
    for (int i = 0; i < n; i++) {
        printf("Intersection %d: (%.6f, %.6f, %.6f)\n", i + 1,
               results[i].point.x, results[i].point.y, results[i].point.z);
        printf("  t1 = %.6f, t2 = %.6f\n", results[i].t1, results[i].t2);
    }
    
    // Cleanup
    free(curve1.control_points);
    free(curve1.weights);
    free(curve1.knots);
    free(curve2.control_points);
    free(curve2.weights);
    free(curve2.knots);
    
    return 0;
}
