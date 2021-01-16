(define (problem ijrr_7)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street12 - street
		building8 building25 building26 building29 - building
	)
	(:init
		(atLocation survivor0 street12)
		(atLocation survivor1 building25)
		(onFire building8)
		(onFire building26)
		(underRubble street12)
		(underRubble building29)
		(needsRepair building8)
		(needsRepair building26)
		(needsRepair building29)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street12))
		(not (needsRepair building8))
		(not (needsRepair building26))
		(not (needsRepair building29))
	))
	(:metric minimize total-time)
)
