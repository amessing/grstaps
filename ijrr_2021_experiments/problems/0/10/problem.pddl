(define (problem ijrr_10)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street31 street35 - street
		building0 building3 building6 building25 - building
	)
	(:init
		(atLocation survivor0 street35)
		(atLocation survivor1 building3)
		(onFire building6)
		(onFire building25)
		(underRubble street31)
		(underRubble building0)
		(needsRepair building0)
		(needsRepair building6)
		(needsRepair building25)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street31))
		(not (needsRepair building0))
		(not (needsRepair building6))
		(not (needsRepair building25))
	))
	(:metric minimize total-time)
)
