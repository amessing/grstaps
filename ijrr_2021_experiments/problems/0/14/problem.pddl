(define (problem ijrr_14)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street6 street35 - street
		building0 building1 building13 - building
	)
	(:init
		(atLocation survivor0 street35)
		(atLocation survivor1 building0)
		(onFire building0)
		(onFire building13)
		(underRubble street6)
		(underRubble building1)
		(needsRepair building0)
		(needsRepair building1)
		(needsRepair building13)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street6))
		(not (needsRepair building0))
		(not (needsRepair building1))
		(not (needsRepair building13))
	))
	(:metric minimize total-time)
)
