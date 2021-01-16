(define (problem ijrr_10)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street7 street11 - street
		building7 building11 building13 - building
	)
	(:init
		(atLocation survivor0 street7)
		(atLocation survivor1 building11)
		(onFire building7)
		(onFire building13)
		(underRubble street11)
		(underRubble building7)
		(needsRepair building7)
		(needsRepair building13)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street11))
		(not (needsRepair building7))
		(not (needsRepair building13))
	))
	(:metric minimize total-time)
)
