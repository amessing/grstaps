(define (problem ijrr_8)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street2 street17 - street
		building3 building23 building27 building30 - building
	)
	(:init
		(atLocation survivor0 street17)
		(atLocation survivor1 building27)
		(onFire building23)
		(onFire building30)
		(underRubble street2)
		(underRubble building3)
		(needsRepair building3)
		(needsRepair building23)
		(needsRepair building30)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street2))
		(not (needsRepair building3))
		(not (needsRepair building23))
		(not (needsRepair building30))
	))
	(:metric minimize total-time)
)
