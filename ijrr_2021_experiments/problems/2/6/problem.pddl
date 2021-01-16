(define (problem ijrr_6)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street0 street25 - street
		building7 building14 building23 building33 - building
	)
	(:init
		(atLocation survivor0 street25)
		(atLocation survivor1 building7)
		(onFire building14)
		(onFire building23)
		(underRubble street0)
		(underRubble building33)
		(needsRepair building14)
		(needsRepair building23)
		(needsRepair building33)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street0))
		(not (needsRepair building14))
		(not (needsRepair building23))
		(not (needsRepair building33))
	))
	(:metric minimize total-time)
)
