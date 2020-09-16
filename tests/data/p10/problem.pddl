(define (problem p10)
    (:domain p10)
    (:objects b1 b2 b3 b4 - box
              gs as bs bt - location)
    (:init
        (loc b1 bs)
        (loc b2 bs)
        (loc b3 bs)
        (loc b4 bs)
    )
    (:goal
        (and 
            (loc b1 bt)
            (loc b2 bt)
            (loc b3 bt)
            (loc b4 bt)
        )
    )
    (:metric minimize (total-time))
)
