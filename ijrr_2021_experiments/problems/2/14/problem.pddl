(define (problem ijrr_14)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street12 street42 - street
		building9 building24 building31 - building
	)
	(:init
		(atLocation survivor0 street12)
		(atLocation survivor1 building31)
		(onFire building24)
		(onFire building31)
		(underRubble street42)
		(underRubble building9)
		(needsRepair building9)
		(needsRepair building24)
		(needsRepair building31)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street42))
		(not (needsRepair building9))
		(not (needsRepair building24))
		(not (needsRepair building31))
	))
	(:metric minimize total-time)
)
