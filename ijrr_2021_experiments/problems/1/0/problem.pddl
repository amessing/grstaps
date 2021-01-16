(define (problem ijrr_0)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street2 street29 - street
		building5 building18 building19 building21 - building
	)
	(:init
		(atLocation survivor0 street2)
		(atLocation survivor1 building18)
		(onFire building19)
		(onFire building21)
		(underRubble street29)
		(underRubble building5)
		(needsRepair building5)
		(needsRepair building19)
		(needsRepair building21)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street29))
		(not (needsRepair building5))
		(not (needsRepair building19))
		(not (needsRepair building21))
	))
	(:metric minimize total-time)
)
