(define (problem ijrr_5)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street19 street23 - street
		building14 building18 building27 building32 - building
	)
	(:init
		(atLocation survivor0 street23)
		(atLocation survivor1 building18)
		(onFire building27)
		(onFire building32)
		(underRubble street19)
		(underRubble building14)
		(needsRepair building14)
		(needsRepair building27)
		(needsRepair building32)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street19))
		(not (needsRepair building14))
		(not (needsRepair building27))
		(not (needsRepair building32))
	))
	(:metric minimize total-time)
)
