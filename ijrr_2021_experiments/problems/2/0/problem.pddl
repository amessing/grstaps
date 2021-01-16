(define (problem ijrr_0)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street49 street57 - street
		building2 building21 building32 building34 - building
	)
	(:init
		(atLocation survivor0 street57)
		(atLocation survivor1 building34)
		(onFire building2)
		(onFire building21)
		(underRubble street49)
		(underRubble building32)
		(needsRepair building2)
		(needsRepair building21)
		(needsRepair building32)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street49))
		(not (needsRepair building2))
		(not (needsRepair building21))
		(not (needsRepair building32))
	))
	(:metric minimize total-time)
)
