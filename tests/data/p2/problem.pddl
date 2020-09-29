(define (problem p2)
    (:domain p2)
    (:objects b1 b2 b3 - box
              l1 l2 - location)
    (:init
        (loc b1 l1)
        (loc b2 l1)
        (loc b3 l1)
    )
    (:goal
        (and 
            (loc b1 l2)
            (loc b2 l2)
            (loc b3 l2)
        )
    )
    (:metric minimize (total-time))
)
