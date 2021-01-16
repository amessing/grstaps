(define (problem ijrr_13)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street5 street29 - street
		building15 building26 building27 - building
	)
	(:init
		(atLocation survivor0 street5)
		(atLocation survivor1 building15)
		(onFire building15)
		(onFire building27)
		(underRubble street29)
		(underRubble building26)
		(needsRepair building15)
		(needsRepair building26)
		(needsRepair building27)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street29))
		(not (needsRepair building15))
		(not (needsRepair building26))
		(not (needsRepair building27))
	))
	(:metric minimize total-time)
)
