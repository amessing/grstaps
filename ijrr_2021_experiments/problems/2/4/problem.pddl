(define (problem ijrr_4)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street3 street39 - street
		building0 building7 building19 building26 - building
	)
	(:init
		(atLocation survivor0 street3)
		(atLocation survivor1 building26)
		(onFire building0)
		(onFire building7)
		(underRubble street39)
		(underRubble building19)
		(needsRepair building0)
		(needsRepair building7)
		(needsRepair building19)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street39))
		(not (needsRepair building0))
		(not (needsRepair building7))
		(not (needsRepair building19))
	))
	(:metric minimize total-time)
)
