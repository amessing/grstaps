(define (problem ijrr_0)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street17 street39 - street
		building13 building23 building24 building25 - building
	)
	(:init
		(atLocation survivor0 street39)
		(atLocation survivor1 building25)
		(onFire building13)
		(onFire building23)
		(underRubble street17)
		(underRubble building24)
		(needsRepair building13)
		(needsRepair building23)
		(needsRepair building24)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street17))
		(not (needsRepair building13))
		(not (needsRepair building23))
		(not (needsRepair building24))
	))
	(:metric minimize total-time)
)
