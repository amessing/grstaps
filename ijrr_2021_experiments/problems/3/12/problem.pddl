(define (problem ijrr_12)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street1 street37 - street
		building13 building14 building19 building23 - building
	)
	(:init
		(atLocation survivor0 street1)
		(atLocation survivor1 building13)
		(onFire building14)
		(onFire building23)
		(underRubble street37)
		(underRubble building19)
		(needsRepair building14)
		(needsRepair building19)
		(needsRepair building23)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street37))
		(not (needsRepair building14))
		(not (needsRepair building19))
		(not (needsRepair building23))
	))
	(:metric minimize total-time)
)
