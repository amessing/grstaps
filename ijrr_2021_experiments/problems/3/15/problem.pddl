(define (problem ijrr_15)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street6 street9 - street
		building2 building24 building30 building31 - building
	)
	(:init
		(atLocation survivor0 street6)
		(atLocation survivor1 building31)
		(onFire building2)
		(onFire building30)
		(underRubble street9)
		(underRubble building24)
		(needsRepair building2)
		(needsRepair building24)
		(needsRepair building30)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street9))
		(not (needsRepair building2))
		(not (needsRepair building24))
		(not (needsRepair building30))
	))
	(:metric minimize total-time)
)
