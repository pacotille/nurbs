(define-library (nurbs)
  (export Point3D? make-Point3D
          Point3D-x Point3D-y Point3D-z
          Point3D-x-set! Point3D-y-set! Point3D-z-set!

          NURBSCurve?
          NURBSCurve-degree NURBSCurve-degree-set!
          NURBSCurve-n-knots NURBSCurve-n-knots-set!

          Intersection
          make-line-curve)
  (import (chibi))
  (include-shared "nurbs"))
