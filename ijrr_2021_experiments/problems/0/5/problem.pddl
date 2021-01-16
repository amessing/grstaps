(define (problem ijrr_5)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street6 street10 - street
		building0 building3 building13 building31 - building
	)
	(:init
		(atLocation survivor0 street6)
		(atLocation survivor1 building13)
		(onFire building0)
		(onFire building31)
		(underRubble street10)
		(underRubble building3)
		(needsRepair building0)
		(needsRepair building3)
		(needsRepair building31)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street10))
		(not (needsRepair building0))
		(not (needsRepair building3))
		(not (needsRepair building31))
	))
	(:metric minimize total-time)
)
