(define (problem ijrr_11)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street15 street31 - street
		building5 building9 building18 building31 - building
	)
	(:init
		(atLocation survivor0 street31)
		(atLocation survivor1 building5)
		(onFire building9)
		(onFire building18)
		(underRubble street15)
		(underRubble building31)
		(needsRepair building9)
		(needsRepair building18)
		(needsRepair building31)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street15))
		(not (needsRepair building9))
		(not (needsRepair building18))
		(not (needsRepair building31))
	))
	(:metric minimize total-time)
)
