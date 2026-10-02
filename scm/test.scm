(import (scheme base) (scheme write) (scheme process-context) (nurbs))

;; ligne horizontale y=0, 6 points de controle, degre 3
;; -> n_knots = n_ctrl + degree + 1 = 6+3+1 = 10
(define c1 (make-line-curve 3 6 10 0.0 0.0))

;; ligne diagonale qui croise y=0 vers le milieu
(define c2 (make-line-curve 3 6 10 -2.5 1.0))

(define p (Intersection c1 c2))

(display "Intersection (approx): ")
(display (list (Point3D-x p) (Point3D-y p) (Point3D-z p)))
(newline)

(if (< (abs (Point3D-y p)) 1.0)
    (begin (display "TEST OK: point proche du croisement attendu") (newline))
    (begin (display "TEST FAILED") (newline) (exit 1)))
