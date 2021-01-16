(define (domain ijrr)
    (:requirements :adl :durative-actions :typing)
    (:types 
        location survivor - object
        street building - location)
    (:predicates
        (atLocation ?s - survivor ?l - location)
        (onFire ?l - location)
        (underRubble ?l - location)
        (needsRepair ?b - building)
    )
    (:constants
        HOSPITAL - building
    )

    ; Move survivor between location
    (:durative-action moveSurvivorToHospital
        :parameters (?s - survivor ?from - location)
        :duration (= ?duration 0.5)
        :condition (and
            ; Not already at hospital
            (at start (not (= ?from HOSPITAL)))
            ; Survivor is at the from location
            (at start (atLocation ?s ?from))
            ; Fire must be put out first
            (at start (not (onFire ?from)))
            ; Rubble must be removed first
            (at start (not (underRubble ?from)))

        )
        :effect (and 
            (at start (not (atLocation ?s ?from)))
            (at end (atLocation ?s HOSPITAL))
        )
    )
    
    ; Put out fire
    (:durative-action exstinguishFire
        :parameters (?l - location)
        :duration (= ?duration 3.0)
        :condition (and 
            (at start (onFire ?l))
        )
        :effect (and 
            (at end (not (onFire ?l)))
        )
    )
    
    ; Remove rubble
    (:durative-action clearRubble
        :parameters (?l - location)
        :duration (= ?duration 5.0)
        :condition (and 
            (at start (underRubble ?l))
            (at start (not (onFire ?l)))
        )
        :effect (and 
            (at end (not (underRubble ?l)))
        )
    )

    ; Repair building
    (:durative-action repairBuilding
        :parameters (?b - building)
        :duration (= ?duration 10.0)
        :condition (and 
            ; Fire must be put out first
            (at start (not (onFire ?b)))
            ; Rubble must be removed first
            (at start (not (underRubble ?b)))
            (at start (needsRepair ?b))
        )
        :effect (and 
            (at end (not (needsRepair ?b)))
        )
    )
    
)