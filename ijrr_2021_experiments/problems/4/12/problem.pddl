(define (problem ijrr_12)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street39 - street
		building9 building15 building24 building28 - building
	)
	(:init
		(atLocation survivor0 street39)
		(atLocation survivor1 building9)
		(onFire building24)
		(onFire building28)
		(underRubble street39)
		(underRubble building15)
		(needsRepair building15)
		(needsRepair building24)
		(needsRepair building28)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street39))
		(not (needsRepair building15))
		(not (needsRepair building24))
		(not (needsRepair building28))
	))
	(:metric minimize total-time)
)
