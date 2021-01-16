(define (problem ijrr_3)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street16 street37 - street
		building6 building8 building9 building19 - building
	)
	(:init
		(atLocation survivor0 street37)
		(atLocation survivor1 building8)
		(onFire building6)
		(onFire building19)
		(underRubble street16)
		(underRubble building9)
		(needsRepair building6)
		(needsRepair building9)
		(needsRepair building19)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street16))
		(not (needsRepair building6))
		(not (needsRepair building9))
		(not (needsRepair building19))
	))
	(:metric minimize total-time)
)
