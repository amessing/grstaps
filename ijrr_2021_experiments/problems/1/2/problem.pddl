(define (problem ijrr_2)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street31 street51 - street
		building1 building19 building21 building32 - building
	)
	(:init
		(atLocation survivor0 street31)
		(atLocation survivor1 building21)
		(onFire building19)
		(onFire building32)
		(underRubble street51)
		(underRubble building1)
		(needsRepair building1)
		(needsRepair building19)
		(needsRepair building32)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street51))
		(not (needsRepair building1))
		(not (needsRepair building19))
		(not (needsRepair building32))
	))
	(:metric minimize total-time)
)
