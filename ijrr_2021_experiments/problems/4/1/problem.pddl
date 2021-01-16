(define (problem ijrr_1)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street18 street21 - street
		building4 building12 building13 building25 - building
	)
	(:init
		(atLocation survivor0 street21)
		(atLocation survivor1 building12)
		(onFire building13)
		(onFire building25)
		(underRubble street18)
		(underRubble building4)
		(needsRepair building4)
		(needsRepair building13)
		(needsRepair building25)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street18))
		(not (needsRepair building4))
		(not (needsRepair building13))
		(not (needsRepair building25))
	))
	(:metric minimize total-time)
)
