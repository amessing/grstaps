(define (problem ijrr_13)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street6 street48 - street
		building12 building13 building14 building29 - building
	)
	(:init
		(atLocation survivor0 street6)
		(atLocation survivor1 building29)
		(onFire building13)
		(onFire building14)
		(underRubble street48)
		(underRubble building12)
		(needsRepair building12)
		(needsRepair building13)
		(needsRepair building14)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street48))
		(not (needsRepair building12))
		(not (needsRepair building13))
		(not (needsRepair building14))
	))
	(:metric minimize total-time)
)
