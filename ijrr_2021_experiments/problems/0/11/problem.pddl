(define (problem ijrr_11)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street19 street41 - street
		building10 building13 building27 building31 - building
	)
	(:init
		(atLocation survivor0 street41)
		(atLocation survivor1 building31)
		(onFire building10)
		(onFire building27)
		(underRubble street19)
		(underRubble building13)
		(needsRepair building10)
		(needsRepair building13)
		(needsRepair building27)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street19))
		(not (needsRepair building10))
		(not (needsRepair building13))
		(not (needsRepair building27))
	))
	(:metric minimize total-time)
)
