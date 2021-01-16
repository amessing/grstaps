(define (problem ijrr_16)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street15 street33 - street
		building9 building20 building22 building36 - building
	)
	(:init
		(atLocation survivor0 street15)
		(atLocation survivor1 building9)
		(onFire building22)
		(onFire building36)
		(underRubble street33)
		(underRubble building20)
		(needsRepair building20)
		(needsRepair building22)
		(needsRepair building36)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street33))
		(not (needsRepair building20))
		(not (needsRepair building22))
		(not (needsRepair building36))
	))
	(:metric minimize total-time)
)
