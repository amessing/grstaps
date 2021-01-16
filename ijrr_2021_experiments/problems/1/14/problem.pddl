(define (problem ijrr_14)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street11 street35 - street
		building2 building3 building28 - building
	)
	(:init
		(atLocation survivor0 street11)
		(atLocation survivor1 building28)
		(onFire building2)
		(onFire building3)
		(underRubble street35)
		(underRubble building2)
		(needsRepair building2)
		(needsRepair building3)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street35))
		(not (needsRepair building2))
		(not (needsRepair building3))
	))
	(:metric minimize total-time)
)
