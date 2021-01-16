(define (problem ijrr_19)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street0 street18 - street
		building12 building20 building24 building25 - building
	)
	(:init
		(atLocation survivor0 street0)
		(atLocation survivor1 building20)
		(onFire building24)
		(onFire building25)
		(underRubble street18)
		(underRubble building12)
		(needsRepair building12)
		(needsRepair building24)
		(needsRepair building25)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street18))
		(not (needsRepair building12))
		(not (needsRepair building24))
		(not (needsRepair building25))
	))
	(:metric minimize total-time)
)
