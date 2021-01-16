(define (problem ijrr_8)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street7 street24 - street
		building5 building10 building29 - building
	)
	(:init
		(atLocation survivor0 street24)
		(atLocation survivor1 building29)
		(onFire building5)
		(onFire building10)
		(underRubble street7)
		(underRubble building10)
		(needsRepair building5)
		(needsRepair building10)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street7))
		(not (needsRepair building5))
		(not (needsRepair building10))
	))
	(:metric minimize total-time)
)
