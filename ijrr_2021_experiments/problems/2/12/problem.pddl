(define (problem ijrr_12)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street24 street38 - street
		building5 building8 building25 building26 - building
	)
	(:init
		(atLocation survivor0 street38)
		(atLocation survivor1 building5)
		(onFire building25)
		(onFire building26)
		(underRubble street24)
		(underRubble building8)
		(needsRepair building8)
		(needsRepair building25)
		(needsRepair building26)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street24))
		(not (needsRepair building8))
		(not (needsRepair building25))
		(not (needsRepair building26))
	))
	(:metric minimize total-time)
)
