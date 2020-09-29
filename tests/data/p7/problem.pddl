(define (problem p7)
    (:domain p7)
    (:objects b1 b2 - box
              l1 l2 - location)
    (:init
        (loc b1 l1)
        (loc b2 l1)
    )
    (:goal
        (and 
            (loc b1 l2)
            (loc b2 l2)
        )
    )
    (:metric minimize (total-time))
)
