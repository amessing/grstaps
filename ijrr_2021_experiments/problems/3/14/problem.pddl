(define (problem ijrr_14)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street18 street46 - street
		building5 building20 building21 building25 - building
	)
	(:init
		(atLocation survivor0 street18)
		(atLocation survivor1 building20)
		(onFire building21)
		(onFire building25)
		(underRubble street46)
		(underRubble building5)
		(needsRepair building5)
		(needsRepair building21)
		(needsRepair building25)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street46))
		(not (needsRepair building5))
		(not (needsRepair building21))
		(not (needsRepair building25))
	))
	(:metric minimize total-time)
)
