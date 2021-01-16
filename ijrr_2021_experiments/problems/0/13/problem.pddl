(define (problem ijrr_13)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street44 street55 - street
		building6 building12 building34 - building
	)
	(:init
		(atLocation survivor0 street55)
		(atLocation survivor1 building12)
		(onFire building6)
		(onFire building34)
		(underRubble street44)
		(underRubble building6)
		(needsRepair building6)
		(needsRepair building34)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street44))
		(not (needsRepair building6))
		(not (needsRepair building34))
	))
	(:metric minimize total-time)
)
