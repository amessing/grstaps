(define (problem ijrr_7)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street7 street49 - street
		building3 building8 building12 building25 - building
	)
	(:init
		(atLocation survivor0 street49)
		(atLocation survivor1 building25)
		(onFire building8)
		(onFire building12)
		(underRubble street7)
		(underRubble building3)
		(needsRepair building3)
		(needsRepair building8)
		(needsRepair building12)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street7))
		(not (needsRepair building3))
		(not (needsRepair building8))
		(not (needsRepair building12))
	))
	(:metric minimize total-time)
)
