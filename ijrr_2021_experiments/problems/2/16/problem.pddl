(define (problem ijrr_16)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street20 street25 - street
		building0 building2 building13 building26 - building
	)
	(:init
		(atLocation survivor0 street20)
		(atLocation survivor1 building2)
		(onFire building0)
		(onFire building13)
		(underRubble street25)
		(underRubble building26)
		(needsRepair building0)
		(needsRepair building13)
		(needsRepair building26)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street25))
		(not (needsRepair building0))
		(not (needsRepair building13))
		(not (needsRepair building26))
	))
	(:metric minimize total-time)
)
