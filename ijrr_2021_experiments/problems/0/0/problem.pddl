(define (problem ijrr_0)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street7 street11 - street
		building6 building13 building14 building30 - building
	)
	(:init
		(atLocation survivor0 street11)
		(atLocation survivor1 building14)
		(onFire building13)
		(onFire building30)
		(underRubble street7)
		(underRubble building6)
		(needsRepair building6)
		(needsRepair building13)
		(needsRepair building30)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street7))
		(not (needsRepair building6))
		(not (needsRepair building13))
		(not (needsRepair building30))
	))
	(:metric minimize total-time)
)
