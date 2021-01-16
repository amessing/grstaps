(define (problem ijrr_6)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street12 street32 - street
		building5 building19 building24 building31 - building
	)
	(:init
		(atLocation survivor0 street32)
		(atLocation survivor1 building5)
		(onFire building19)
		(onFire building24)
		(underRubble street12)
		(underRubble building31)
		(needsRepair building19)
		(needsRepair building24)
		(needsRepair building31)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street12))
		(not (needsRepair building19))
		(not (needsRepair building24))
		(not (needsRepair building31))
	))
	(:metric minimize total-time)
)
