(define (problem ijrr_12)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street14 street44 - street
		building0 building3 building10 building12 - building
	)
	(:init
		(atLocation survivor0 street14)
		(atLocation survivor1 building3)
		(onFire building0)
		(onFire building12)
		(underRubble street44)
		(underRubble building10)
		(needsRepair building0)
		(needsRepair building10)
		(needsRepair building12)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street44))
		(not (needsRepair building0))
		(not (needsRepair building10))
		(not (needsRepair building12))
	))
	(:metric minimize total-time)
)
