(define (problem ijrr_16)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street28 street49 - street
		building3 building9 building14 building24 - building
	)
	(:init
		(atLocation survivor0 street28)
		(atLocation survivor1 building14)
		(onFire building9)
		(onFire building24)
		(underRubble street49)
		(underRubble building3)
		(needsRepair building3)
		(needsRepair building9)
		(needsRepair building24)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street49))
		(not (needsRepair building3))
		(not (needsRepair building9))
		(not (needsRepair building24))
	))
	(:metric minimize total-time)
)
