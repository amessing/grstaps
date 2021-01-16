(define (problem ijrr_9)
	(:domain ijrr)
	(:objects
		survivor0 survivor1 - survivor
		street23 street41 - street
		building15 building16 building23 - building
	)
	(:init
		(atLocation survivor0 street41)
		(atLocation survivor1 building23)
		(onFire building15)
		(onFire building16)
		(underRubble street23)
		(underRubble building16)
		(needsRepair building15)
		(needsRepair building16)
	)
	(:goal (and
		(atLocation survivor0 HOSPITAL)
		(atLocation survivor1 HOSPITAL)
		(not (underRubble street23))
		(not (needsRepair building15))
		(not (needsRepair building16))
	))
	(:metric minimize total-time)
)
