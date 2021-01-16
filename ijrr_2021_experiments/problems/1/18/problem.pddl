(define (problem ijrr_18)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street7 street48 - street
		building2 building13 building26 building29 - building
	)
	(:init
		(atLocation survivor0 street48)
		(atLocation survivor1 building2)
		(onFire building13)
		(onFire building26)
		(underRubble street7)
		(underRubble building29)
		(needsRepair building13)
		(needsRepair building26)
		(needsRepair building29)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street7))
		(not (needsRepair building13))
		(not (needsRepair building26))
		(not (needsRepair building29))
	))
	(:metric minimize total-time)
)
