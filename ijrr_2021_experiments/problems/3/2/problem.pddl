(define (problem ijrr_2)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street22 street50 - street
		building1 building13 building32 - building
	)
	(:init
		(atLocation survivor0 street22)
		(atLocation survivor1 building1)
		(onFire building1)
		(onFire building13)
		(underRubble street50)
		(underRubble building32)
		(needsRepair building1)
		(needsRepair building13)
		(needsRepair building32)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street50))
		(not (needsRepair building1))
		(not (needsRepair building13))
		(not (needsRepair building32))
	))
	(:metric minimize total-time)
)
