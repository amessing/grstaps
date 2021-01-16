(define (problem ijrr_6)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street5 street23 - street
		building3 building25 building33 building34 - building
	)
	(:init
		(atLocation survivor0 street5)
		(atLocation survivor1 building33)
		(onFire building25)
		(onFire building34)
		(underRubble street23)
		(underRubble building3)
		(needsRepair building3)
		(needsRepair building25)
		(needsRepair building34)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street23))
		(not (needsRepair building3))
		(not (needsRepair building25))
		(not (needsRepair building34))
	))
	(:metric minimize total-time)
)
