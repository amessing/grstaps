(define (problem ijrr_3)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street28 street51 - street
		building0 building1 building30 - building
	)
	(:init
		(atLocation survivor0 street51)
		(atLocation survivor1 building30)
		(onFire building0)
		(onFire building1)
		(underRubble street28)
		(underRubble building0)
		(needsRepair building0)
		(needsRepair building1)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street28))
		(not (needsRepair building0))
		(not (needsRepair building1))
	))
	(:metric minimize total-time)
)
