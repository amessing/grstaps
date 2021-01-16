(define (problem ijrr_19)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street54 street55 - street
		building10 building24 building27 - building
	)
	(:init
		(atLocation survivor0 street55)
		(atLocation survivor1 building10)
		(onFire building24)
		(underRubble street54)
		(underRubble building27)
		(needsRepair building24)
		(needsRepair building27)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street54))
		(not (needsRepair building24))
		(not (needsRepair building27))
	))
	(:metric minimize total-time)
)
