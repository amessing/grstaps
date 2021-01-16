(define (problem ijrr_8)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street26 street39 - street
		building6 building9 building12 building20 - building
	)
	(:init
		(atLocation survivor0 street26)
		(atLocation survivor1 building6)
		(onFire building12)
		(onFire building20)
		(underRubble street39)
		(underRubble building9)
		(needsRepair building9)
		(needsRepair building12)
		(needsRepair building20)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street39))
		(not (needsRepair building9))
		(not (needsRepair building12))
		(not (needsRepair building20))
	))
	(:metric minimize total-time)
)
