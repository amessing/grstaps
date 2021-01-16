(define (problem ijrr_9)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street3 street42 - street
		building3 building5 building10 building31 - building
	)
	(:init
		(atLocation survivor0 street42)
		(atLocation survivor1 building10)
		(onFire building3)
		(onFire building31)
		(underRubble street3)
		(underRubble building5)
		(needsRepair building3)
		(needsRepair building5)
		(needsRepair building31)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street3))
		(not (needsRepair building3))
		(not (needsRepair building5))
		(not (needsRepair building31))
	))
	(:metric minimize total-time)
)
