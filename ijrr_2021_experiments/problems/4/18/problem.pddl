(define (problem ijrr_18)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street2 street11 - street
		building8 building11 building28 building30 - building
	)
	(:init
		(atLocation survivor0 street11)
		(atLocation survivor1 building28)
		(onFire building8)
		(onFire building11)
		(underRubble street2)
		(underRubble building30)
		(needsRepair building8)
		(needsRepair building11)
		(needsRepair building30)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street2))
		(not (needsRepair building8))
		(not (needsRepair building11))
		(not (needsRepair building30))
	))
	(:metric minimize total-time)
)
