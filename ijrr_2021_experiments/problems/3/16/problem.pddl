(define (problem ijrr_16)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street39 street57 - street
		building12 building20 building28 building31 - building
	)
	(:init
		(atLocation survivor0 street39)
		(atLocation survivor1 building28)
		(onFire building12)
		(onFire building31)
		(underRubble street57)
		(underRubble building20)
		(needsRepair building12)
		(needsRepair building20)
		(needsRepair building31)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street57))
		(not (needsRepair building12))
		(not (needsRepair building20))
		(not (needsRepair building31))
	))
	(:metric minimize total-time)
)
