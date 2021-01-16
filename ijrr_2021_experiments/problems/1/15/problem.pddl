(define (problem ijrr_15)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street34 street53 - street
		building10 building21 building32 building35 - building
	)
	(:init
		(atLocation survivor0 street34)
		(atLocation survivor1 building35)
		(onFire building10)
		(onFire building32)
		(underRubble street53)
		(underRubble building21)
		(needsRepair building10)
		(needsRepair building21)
		(needsRepair building32)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street53))
		(not (needsRepair building10))
		(not (needsRepair building21))
		(not (needsRepair building32))
	))
	(:metric minimize total-time)
)
