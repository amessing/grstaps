(define (problem ijrr_15)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street19 street21 - street
		building0 building14 building23 building36 - building
	)
	(:init
		(atLocation survivor0 street19)
		(atLocation survivor1 building0)
		(onFire building23)
		(onFire building36)
		(underRubble street21)
		(underRubble building14)
		(needsRepair building14)
		(needsRepair building23)
		(needsRepair building36)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street21))
		(not (needsRepair building14))
		(not (needsRepair building23))
		(not (needsRepair building36))
	))
	(:metric minimize total-time)
)
