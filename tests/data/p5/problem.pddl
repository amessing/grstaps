(define (problem p5)
    (:domain p5)
    (:objects b1 b2 b3 b4 b5 b6 - box
              l1 l2 - location)
    (:init
        (loc b1 l1)
        (loc b2 l1)
        (loc b3 l1)
        (loc b4 l1)
        (loc b5 l1)
        (loc b6 l1)
    )
    (:goal
        (and 
            (loc b1 l2)
            (loc b2 l2)
            (loc b3 l2)
            (loc b4 l2)
            (loc b5 l2)
            (loc b6 l2)
        )
    )
    (:metric minimize (total-time))
)
